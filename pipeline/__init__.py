import dataclasses
import struct
from abc import ABC
from dataclasses import dataclass
from functools import partial
from struct import Struct
from typing import ClassVar, Final, Self

import numpy as np
import numpy.typing as npt

from pipeline._pipeline import (
    CACHE_ALIGNMENT,
    MASK_ALPHA,
    MASK_EMISSION,
    MASK_PALETTE_INDEX,
    MAX_2_BIT,
    MAX_5_BIT,
    MAX_6_BIT,
    MAX_PALETTE_SIZE,
    NEON_ALIGNMENT,
    PACKED_COLOR_SIZE_BYTES,
    PALETTE_SIZE_BYTES,
    ColorEncoding,
    PhysicsType,
)
from pipeline._pipeline import (
    SpriteMetadata as _SpriteMetadata,
)

# Graphics

ASSET_LAYOUT_VERSION: bytes = b"7"

SPRITE_DIMENSIONS_SIZE_BYTES: int = 2
"""The total size of the sprite dimensions in bytes."""

FOOTER_SIZE_BYTES: int = PALETTE_SIZE_BYTES + SPRITE_DIMENSIONS_SIZE_BYTES
""" The total size of the sprite footer in bytes."""

type BGRImage = npt.NDArray[np.uint8]

type Palette = npt.NDArray[np.uint8]


class ResourceLayoutError(Exception): ...


class ResourceLayoutWarning(UserWarning): ...


@dataclass(slots=True)
class Metadata(ABC):
    MAGIC: ClassVar[bytes]
    STRUCT: ClassVar[Struct]
    EXPECTED_SIZE_BYTES: ClassVar[int]

    @classmethod
    def from_bytes(cls, buffer: bytes) -> Self:
        """"""

        expected_size = cls.EXPECTED_SIZE_BYTES
        err_msg = (
            f"Buffer size too small for {cls.__name__}. "
            f"Expected {expected_size}, got"
        )

        buffer_size = len(buffer)
        if buffer_size < expected_size:
            raise ResourceLayoutError(f"{err_msg} {buffer_size}.")

        magic_size = len(cls.MAGIC)
        magic = buffer[:magic_size]
        if magic != cls.MAGIC:
            raise ResourceLayoutError(
                "Invalid magic bytes! "
                f"Expected '{cls.MAGIC}' bytes, got {magic}."
            )

        try:
            return cls(*cls.STRUCT.unpack(buffer[magic_size:expected_size]))
        except struct.error:
            raise ResourceLayoutError(f"{err_msg} {len(buffer)}.")

    def to_bytes(self) -> bytes:
        return self.MAGIC + self.__class__.STRUCT.pack(
            *dataclasses.astuple(self)
        )


class SpriteMetadata(_SpriteMetadata):
    MAGIC: ClassVar[bytes] = b"SC SP v" + ASSET_LAYOUT_VERSION

    EXPECTED_SIZE_BYTES = len(MAGIC) + _SpriteMetadata.EXPECTED_SIZE_BYTES
    PALETTE_INDEX_OFFSET = EXPECTED_SIZE_BYTES - 1

    @classmethod
    def from_bytes(cls, buffer: bytes) -> Self:

        expected_size = super().EXPECTED_SIZE_BYTES
        err_msg = (
            f"Buffer size too small for {cls.__name__}. "
            f"Expected {expected_size}, got"
        )

        buffer_size = len(buffer)
        if buffer_size < expected_size:
            raise ResourceLayoutError(f"{err_msg} {buffer_size}.")

        magic_size = len(cls.MAGIC)
        magic = buffer[:magic_size]
        if magic != cls.MAGIC:
            raise ResourceLayoutError(
                "Invalid magic bytes! "
                f"Expected '{cls.MAGIC}' bytes, got {magic}."
            )

        return super().from_bytes(buffer[magic_size:])

    def to_bytes(self) -> bytes:
        return self.MAGIC + super().to_bytes()


SPRITE_MINIMUM_FILE_SIZE_BYTES: Final[int] = (
    SpriteMetadata.EXPECTED_SIZE_BYTES + FOOTER_SIZE_BYTES
)
"""The minimum file size of a sprite file in bytes."""


@dataclass(slots=True)
class AtlasMetadata(Metadata):
    sprite16_count: int
    sprite32_count: int
    palette_count: int

    MAGIC: ClassVar[Final[bytes]] = b"SC AT v" + ASSET_LAYOUT_VERSION

    STRUCT: ClassVar[Final[Struct]] = Struct("<LHH")
    """The layout of an 8‑byte atlas metadata.

    - magic
    - sprite16_count (uint32)
    - sprite32_count (uint16)
    - palette_count (uint16)
    """

    EXPECTED_SIZE_BYTES: ClassVar[Final[int]] = STRUCT.size + len(MAGIC)


@dataclass(slots=True)
class Sprite(Metadata):
    meta: SpriteMetadata
    pixels: BGRImage
    palette: Palette
    width: int
    height: int

    @classmethod
    def from_bytes(cls, buffer: bytes) -> Self: ...

    def to_bytes(self) -> bytes: ...


# Public functions


def is_power_of_2(n: int) -> bool:
    """
    Check if a number is a power of 2.

    :param n: The number to check.
    :return: `True` if it is; `False` otherwise.
    """

    return n > 0 and n & (n - 1) == 0


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

    :return: The remapped value.
    """
    if src_max == 0x00 or dst_max == 0x00:
        raise ValueError(
            "Source or destination maximum values cannot be 0x00."
        )

    return (value * dst_max + src_max // 2) // src_max


quantize = partial(remap, src_max=0xFF)
dequantize = partial(remap, dst_max=0xFF)
