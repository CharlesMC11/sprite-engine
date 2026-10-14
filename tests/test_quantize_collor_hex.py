from pipeline.preview_colors import Color, hex_str, filter_hex_codes


def test_filter_hex_codes() -> None:
    c = filter_hex_codes(("#000000", "something"))

    assert (0, 0, 0) in c
    assert "something" not in c


def test_hex_str() -> None:
    assert hex_str(1, 2, 3) == "#010203"
    assert hex_str(20, 31, 245) == "#141FF5"


def test_color() -> None:
    color = Color(1, 2, 3)

    r, g, b = color
    assert r == 1
    assert g == 2
    assert b == 3
