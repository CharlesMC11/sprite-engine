import numpy as np
import numpy.typing as npt

from pipeline._pipeline import (
    ALIGNMENT_CACHE,
    ALIGNMENT_NEON,
    ATLAS_MAGIC_BYTES,
    FRACTIONAL_BITS,
    MASK_ALPHA,
    MASK_EMISSION,
    MASK_PALETTE_INDEX,
    MAX_2BIT,
    MAX_4BIT,
    MAX_5BIT,
    MAX_6BIT,
    MAX_COLOR_COUNT,
    MAX_ROM_SIZE,
    PACKED_COLOR_SIZE_BYTES,
    PALETTE_SIZE_BYTES,
    SHIFT_ALPHA,
    SHIFT_EMISSION,
    AtlasMetadata,
    BBox,
    ColorEncoding,
    ColorEncodingParams,
    PhysicsType,
    SpriteMetadata,
    convert_to_fixed_point,
)

# Core

MAX_8BIT: int = 0xFF

# Graphics

type BGRImage = npt.NDArray[np.uint8]
type Palette = npt.NDArray[np.uint8]


SPRITE_DIMENSIONS_SIZE_BYTES: int = 2
"""The total size of the sprite dimensions in bytes."""

FOOTER_SIZE_BYTES: int = PALETTE_SIZE_BYTES + SPRITE_DIMENSIONS_SIZE_BYTES
""" The total size of the sprite footer in bytes."""


class ResourceLayoutError(Exception): ...


class ResourceLayoutWarning(UserWarning): ...


SPRITE_MINIMUM_FILE_SIZE_BYTES: int = (
    SpriteMetadata.EXPECTED_SIZE_BYTES + FOOTER_SIZE_BYTES
)
"""The minimum file size of a sprite file in bytes."""


# Public functions


def is_power_of_2(n: int) -> bool:
    """
    Check if a number is a power of 2.

    :param n: The number to check.
    :return: `True` if it is; `False` otherwise.
    """

    return n > 0 and n.bit_count() == 1


def calculate_padding_needed(n: int, alignment: int) -> int:
    """
    Calculate the needed padding in terms of bytes.

    :param n: The number to be padded.
    :param alignment: The value to be padded up to.

    :return: The number of bytes needed.
    """

    if not is_power_of_2(alignment):
        raise ValueError(f"Alignment must be a power of 2, got {alignment}.")

    return (alignment - n % alignment) % alignment


def remap[T: (int, npt.NDArray)](value: T, /, src_max: int, dst_max: int) -> T:
    """
    Remap a value from one bit depth to another.

    :param value: The value to remap.
    :param src_max: The maximum value for the source bit depth.
    :param dst_max: The maximum value for the destination bit depth.

    :return: The remapped value/s.
    """
    if not src_max or not dst_max:
        raise ValueError(
            "Source or destination maximum values cannot be 0x00."
        )

    return (value * dst_max + (src_max // 2)) // src_max


def remap_from_8bit[T: (int, npt.NDArray)](value: T, dst_max: int) -> T:
    """
    Remap a value or tensor from 8-bit to another bit depth.

    :param value: The value/s to remap.
    :param dst_max: The maximum value for the destination bit depth.

    :return: The remapped value/s.
    """

    return remap(value, MAX_8BIT, dst_max)


def remap_to_8bit[T: (int, npt.NDArray)](value: T, src_max: int) -> T:
    """
    Remap a value or tensor from one bit depth to 8-bit.

    :param value: The value/s to remap.
    :param src_max: The maximum value for the source bit depth.

    :return: The remapped value/s.
    """

    return remap(value, src_max, MAX_8BIT)
