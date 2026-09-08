"""
id_codec.py

Encodes/decodes the numeric text-ID scheme used for the settings-page
localization sheet into/from a readable JSON structure.

------------------------------------------------------------------
ID FORMAT
------------------------------------------------------------------
All settings-page IDs are 5 digits: PPBBB
  PP  = page prefix   = 63 + (page_number - 1)     [63, 64, 65, 66, ...]
  BBB = local code, meaning depends on the block:

  Radios (2-4 option "buttons"), radio index r (0-based), option index o:
    Name          : 00{r}
    Option name   : 0{r+1}{o}
    Option desc   : 1{r+1}{o}

  Scrollers (any number of options), scroller index s (0-based):
    Name          : 70{s}
    Option name   : 7{s+1}{o}
    Option desc   : {H}{T}{o}   <-- NOT purely derivable from s.
                                    In the source data this landed in
                                    whatever 8xx/9xx block was free, so
                                    we store the (H, T) "desc_slot" for
                                    each scroller explicitly in the JSON
                                    instead of computing it.

  Page title / description block uses a DIFFERENT prefix (62f):
    Title          : 62f0{page}      (1-indexed)
    Description    : 62f1{page}      (1-indexed)
    Title (dupe)   : 62f2{page-1}    (0-indexed!! matches source data)

Note: "62f01" etc. are shown here as strings because of the literal
'f' in the middle of the ID (not a real hex digit, just how the sheet
names them) -- we keep them as strings throughout.
------------------------------------------------------------------
"""

import json


def page_prefix(page: int) -> int:
    """Return the 5-digit base (e.g. page 1 -> 63000) for a page number."""
    return 63000 + (page - 1) * 1000


# ---------------------------------------------------------------------
# ENCODERS (JSON -> ID)
# ---------------------------------------------------------------------

def id_title(page: int) -> str:
    return f"62f0{page}"


def id_desc(page: int) -> str:
    return f"62f1{page}"


def id_title_dupe(page: int) -> str:
    return f"62f2{page - 1}"


def id_radio_name(page: int, r: int) -> int:
    return page_prefix(page) + 0 * 100 + 0 * 10 + r


def id_radio_option_name(page: int, r: int, o: int) -> int:
    return page_prefix(page) + 0 * 100 + (r + 1) * 10 + o


def id_radio_option_desc(page: int, r: int, o: int) -> int:
    # NOTE: unlike option-name (T = r+1), the desc block uses T = r (0-indexed).
    # Confirmed from source: FPS is radio index 0 -> descs are 63100-63103 (T=0),
    # not 63110-63113.
    return page_prefix(page) + 1 * 100 + r * 10 + o


def id_scroller_name(page: int, s: int) -> int:
    return page_prefix(page) + 7 * 100 + 0 * 10 + s


def id_scroller_option_name(page: int, s: int, o: int) -> int:
    return page_prefix(page) + 7 * 100 + (s + 1) * 10 + o


def id_scroller_option_desc(page: int, h: int, t: int, o: int) -> int:
    """h/t come from the scroller's explicit 'desc_slot' in the JSON,
    since this block isn't derivable purely from the scroller index."""
    return page_prefix(page) + h * 100 + t * 10 + o


# ---------------------------------------------------------------------
# JSON -> flat {id: text} dict
# ---------------------------------------------------------------------

def encode_page(page_obj: dict) -> dict:
    out = {}
    page = page_obj["page"]

    title = page_obj.get("title")
    if title:
        out[id_title(page)] = title.get("text", "")
        if "dupe" in title:
            out[id_title_dupe(page)] = title["dupe"]
    if page_obj.get("desc"):
        out[id_desc(page)] = page_obj["desc"]

    for r, radio in enumerate(page_obj.get("radios", [])):
        out[id_radio_name(page, r)] = radio["name"]
        for o, opt in enumerate(radio.get("options", [])):
            out[id_radio_option_name(page, r, o)] = opt["name"]
            if opt.get("desc") is not None:
                out[id_radio_option_desc(page, r, o)] = opt["desc"]

    for s, scroller in enumerate(page_obj.get("scrollers", [])):
        out[id_scroller_name(page, s)] = scroller["name"]
        slot = scroller.get("desc_slot")  # {"h":8,"t":0} or None
        for o, opt in enumerate(scroller.get("options", [])):
            out[id_scroller_option_name(page, s, o)] = opt["name"]
            if opt.get("desc") is not None and slot is not None:
                out[id_scroller_option_desc(page, slot["h"], slot["t"], o)] = opt["desc"]

    return out


def encode_all(data: dict) -> dict:
    """data = {"pages": [ {...}, {...} ]}. Returns {id(int/str): text}."""
    merged = {}
    for page_obj in data["pages"]:
        merged.update(encode_page(page_obj))
    return merged


# ---------------------------------------------------------------------
# CLI helper: dump merged JSON -> the original "62f01 = ..." text format
# ---------------------------------------------------------------------

def to_id_text(merged: dict) -> str:
    lines = []
    for k, v in merged.items():
        lines.append(f"{k}\t= {v}")
    return "\n".join(lines)


if __name__ == "__main__":
    import sys

    with open("settings.json", "r", encoding="utf-8") as f:
        data = json.load(f)

    merged = encode_all(data)
    text = to_id_text(merged)

    out_path = sys.argv[2] if len(sys.argv) > 2 else "output_ids.txt"
    with open(out_path, "w", encoding="utf-8") as f:
        with open("PulsarBMG.txt", 'r') as src:
            for line in src:
                f.write(line)
        f.write(text)

    print(f"Wrote {len(merged)} IDs to {out_path}")
