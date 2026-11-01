import dataclasses
from collections.abc import Callable
from dataclasses import dataclass
from functools import partial
from struct import Struct
from typing import ClassVar, Final, Protocol, Self

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
    BBox,
    ColorEncoding,
    PhysicsType,
)
from pipeline._pipeline import (
    SpriteMetadata as _SpriteMetadata,
)

# Graphics

type BGRImage = npt.NDArray[np.uint8]
type Palette = npt.NDArray[np.uint8]

ASSET_LAYOUT_VERSION: bytes = b"7"

SPRITE_DIMENSIONS_SIZE_BYTES: int = 2
"""The total size of the sprite dimensions in bytes."""

FOOTER_SIZE_BYTES: int = PALETTE_SIZE_BYTES + SPRITE_DIMENSIONS_SIZE_BYTES
""" The total size of the sprite footer in bytes."""


class ResourceLayoutError(Exception): ...


class ResourceLayoutWarning(UserWarning): ...


class Serializable(Protocol):
    @classmethod
    def from_bytes(cls, buffer: bytes) -> Self: ...

    def to_bytes(self) -> bytes: ...


class Metadata:
    # Type annotations

    _internal: Serializable
    _from_bytes: ClassVar[Callable[[bytes], Self]]

    # Public class constants

    MAGIC: ClassVar[bytes]
    STRUCT: ClassVar[Struct]
    EXPECTED_SIZE_BYTES: ClassVar[int]

    # Public class methods

    @classmethod
    def from_bytes(cls, buffer: bytes) -> Self:

        expected_size = cls.EXPECTED_SIZE_BYTES
        err_msg = (
            f"Buffer size too small for {cls.__name__}. "
            f"Expected {expected_size} bytes, got"
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
            return cls._from_bytes(buffer[magic_size:expected_size])
        except Exception:
            raise ResourceLayoutError(f"{err_msg} {len(buffer)}.")

    # Public methods

    def to_bytes(self) -> bytes:
        return self.MAGIC + self._internal.to_bytes()


class SpriteMetadata(Metadata):
    __doc__ = _SpriteMetadata.__doc__

    # Type annotations

    _internal: _SpriteMetadata
    _from_bytes = _SpriteMetadata.from_bytes

    # Public class constants

    MAGIC: ClassVar[bytes] = b"SC SP v" + ASSET_LAYOUT_VERSION
    EXPECTED_SIZE_BYTES = len(MAGIC) + _SpriteMetadata.EXPECTED_SIZE_BYTES
    PALETTE_INDEX_OFFSET = EXPECTED_SIZE_BYTES - 1

    # Magic methods

    def __init__(
        self,
        u_min: int,
        u_max: int,
        v_min: int,
        v_max: int,
        u_anchor: int,
        v_anchor: int,
        depth: int,
        physics_type: PhysicsType,
        color_encoding: ColorEncoding,
        palette_index: int = 0x3F,
    ):
        self._internal = _SpriteMetadata(
            u_min,
            u_max,
            v_min,
            v_max,
            u_anchor,
            v_anchor,
            depth,
            physics_type,
            color_encoding,
            palette_index,
        )

    # Properties

    @property
    def bbox(self) -> BBox:
        return self._internal.bbox

    @property
    def u_anchor(self) -> int:
        return self._internal.u_anchor

    @property
    def v_anchor(self) -> int:
        return self._internal.v_anchor

    @property
    def depth(self) -> int:
        return self._internal.depth

    @property
    def physics_type(self) -> PhysicsType:
        return self._internal.physics_type

    @property
    def color_encoding(self) -> ColorEncoding:
        return self._internal.color_encoding

    @property
    def palette_index(self) -> int:
        return self._internal.palette_index

    @palette_index.setter
    def palette_index(self, index: int) -> None:
        self._internal.palette_index = index


SPRITE_MINIMUM_FILE_SIZE_BYTES: Final[int] = (
    SpriteMetadata.EXPECTED_SIZE_BYTES + FOOTER_SIZE_BYTES
)
"""The minimum file size of a sprite file in bytes."""


@dataclass(slots=True)
class AtlasMetadata(Metadata):
    # Type annotations

    sprite16_count: int
    sprite32_count: int
    palette_count: int

    # Public class constants

    MAGIC: ClassVar[Final[bytes]] = b"SC AT v" + ASSET_LAYOUT_VERSION

    STRUCT: ClassVar[Final[Struct]] = Struct("<LHH")
    """The layout of an 8‑byte atlas metadata.

    - magic
    - sprite16_count (uint32)
    - sprite32_count (uint16)
    - palette_count (uint16)
    """

    EXPECTED_SIZE_BYTES: ClassVar[Final[int]] = STRUCT.size + len(MAGIC)

    def to_bytes(self) -> bytes:
        return self.MAGIC + self.__class__.STRUCT.pack(
            *dataclasses.astuple(self)
        )


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
