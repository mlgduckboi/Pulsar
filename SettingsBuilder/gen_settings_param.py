"""
gen_settings_param.py

Regenerates SettingsParam.cpp's per-page arrays (radioCount, scrollerCount,
buttonsPerPagePerRow, optionsPerPagePerScroller) from settings.json.

The first 5 pages (Menu, Race, Host, OTT, KO) are NOT derived from the JSON
-- they're kept exactly as they already exist in the source file, since
that data isn't represented in settings.json. Only "Test" and "KOVS" are
computed, driven by which JSON page's title matches which page name.

Usage:
    python3 gen_settings_param.py settings.json SettingsParam.cpp
"""

import json
import sys

# ---------------------------------------------------------------------
# Fixed data for the first 5 pages (not derivable from settings.json).
# Copied verbatim from the existing source file.
# ---------------------------------------------------------------------
FIXED_PAGE_NAMES = ["menu", "race", "host", "OTT", "KO"]

FIXED_RADIO_COUNT = [3, 5, 3, 5, 2]
FIXED_SCROLLER_COUNT = [1, 1, 1, 0, 2]

FIXED_BUTTONS_PER_ROW = [
    [2, 2, 3, 0, 0, 0],  # Menu
    [2, 2, 2, 2, 3, 0],  # Race
    [2, 4, 2, 0, 0, 0],  # Host
    [3, 3, 2, 2, 2, 0],  # OTT
    [2, 2, 0, 0, 0, 0],  # KO
]

FIXED_OPTIONS_PER_SCROLLER = [
    [5, 7, 0, 0, 0],  # Menu
    [4, 0, 0, 0, 0],  # Race
    [7, 0, 0, 0, 0],  # Host
    [0, 0, 0, 0, 0],  # OTT
    [4, 4, 0, 0, 0],  # KO
]

# Column widths, taken from the existing arrays (max radios/scrollers a
# page can have). If these ever change in Config.hpp, update here too.
MAX_RADIO_COUNT = 6
MAX_SCROLLER_COUNT = 5

# Trailing C++ page slots are no longer hardcoded here -- every page object
# in settings.json becomes a trailing page, in ascending "page" order. Add,
# remove, rename, or reorder pages freely in the JSON; this script just
# follows along.


def pad(row, width):
    if len(row) > width:
        raise ValueError(f"Row {row} exceeds max width {width}")
    return row + [0] * (width - len(row))


def build_trailing_pages(data):
    """Returns (names, radio_counts, scroller_counts, button_rows, scroller_rows)
    for every page in settings.json, sorted by ascending "page" number."""
    missing_id_pages = [p.get("title", {}).get("text", "?") for p in data["pages"] if "cpp_page" not in p]
    if missing_id_pages:
        raise ValueError(
            f"These JSON pages are missing a \"cpp_page\" field: {missing_id_pages}. "
            f"Add e.g. \"cpp_page\": \"Test\" to each page object so it can be matched "
            f"reliably, independent of the display title."
        )

    dupe_ids = [name for name in {p["cpp_page"] for p in data["pages"]}
                if sum(1 for p in data["pages"] if p["cpp_page"] == name) > 1]
    if dupe_ids:
        raise ValueError(f"Duplicate \"cpp_page\" values found: {dupe_ids}. Each page needs a unique id.")

    ordered_pages = sorted(data["pages"], key=lambda p: p["page"])

    names, radio_counts, scroller_counts = [], [], []
    button_rows, scroller_rows = [], []

    for match in ordered_pages:
        name = match["cpp_page"]
        radios = match.get("radios", [])
        scrollers = match.get("scrollers", [])

        if len(radios) > MAX_RADIO_COUNT:
            raise ValueError(f"{name}: {len(radios)} radios exceeds MAX_RADIO_COUNT={MAX_RADIO_COUNT}")
        if len(scrollers) > MAX_SCROLLER_COUNT:
            raise ValueError(f"{name}: {len(scrollers)} scrollers exceeds MAX_SCROLLER_COUNT={MAX_SCROLLER_COUNT}")

        names.append(name)
        radio_counts.append(len(radios))
        scroller_counts.append(len(scrollers))
        button_rows.append(pad([len(r["options"]) for r in radios], MAX_RADIO_COUNT))
        scroller_rows.append(pad([len(s["options"]) for s in scrollers], MAX_SCROLLER_COUNT))

    return names, radio_counts, scroller_counts, button_rows, scroller_rows


def render_cpp(data):
    names, radio_counts, scroller_counts, button_rows, scroller_rows = build_trailing_pages(data)

    all_names = FIXED_PAGE_NAMES + names
    all_radio_counts = FIXED_RADIO_COUNT + radio_counts
    all_scroller_counts = FIXED_SCROLLER_COUNT + scroller_counts
    all_button_rows = FIXED_BUTTONS_PER_ROW + button_rows
    all_scroller_rows = FIXED_OPTIONS_PER_SCROLLER + scroller_rows

    name_comment = ", ".join(all_names)

    radio_count_line = ", ".join(str(n) for n in all_radio_counts)
    scroller_count_line = ", ".join(str(n) for n in all_scroller_counts)

    button_row_lines = []
    for name, row in zip(all_names, all_button_rows):
        row_str = ", ".join(str(n) for n in row)
        button_row_lines.append(f"    {{ {row_str} }}, //{name}")

    scroller_row_lines = []
    for name, row in zip(all_names, all_scroller_rows):
        row_str = ", ".join(str(n) for n in row)
        scroller_row_lines.append(f"    {{ {row_str}}}, //{name}")

    return f"""#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <Config.hpp>
#include <Settings/SettingsParam.hpp>

namespace Pulsar {{

namespace Settings {{

u8 Params::radioCount[Params::pageCount] ={{
    {radio_count_line} //{name_comment}
    //Add user radio count here

}};
u8 Params::scrollerCount[Params::pageCount] ={{ {scroller_count_line} }}; //{name_comment}

u8 Params::buttonsPerPagePerRow[Params::pageCount][Params::maxRadioCount] = //first row is PulsarSettingsType, 2nd is rowIdx of radio
{{
{chr(10).join(button_row_lines)}
    //{{}}, //User
}};

u8 Params::optionsPerPagePerScroller[Params::pageCount][Params::maxScrollerCount] =
{{
{chr(10).join(scroller_row_lines)}
    //{{}}, //User
}};

}}//namespace Settings
}}//namespace Pulsar
"""


if __name__ == "__main__":
    with open("settings.json", "r", encoding="utf-8") as f:
        data = json.load(f)

    out_path = sys.argv[2] if len(sys.argv) > 2 else "SettingsParam.cpp"
    cpp = render_cpp(data)

    with open(out_path, "w", encoding="utf-8") as f:
        f.write(cpp)

    print(f"Wrote {out_path}")