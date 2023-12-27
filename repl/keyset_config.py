from dataclasses import dataclass
from dataclasses_json import dataclass_json


@dataclass_json
@dataclass
class KeysetOneToOneReplacement:
    before: str
    after: str


# Currently, one_to_many replacement is not supported
# @dataclass_json
# @dataclass
# class KeysetOneToManyReplacement:
#     list: Dict[str, str]


@dataclass_json
@dataclass
class LettersToLowercaseReplacement:
    list: str


@dataclass_json
@dataclass
class KeysetReplacementConfig:
    one_to_one: KeysetOneToOneReplacement
    letters_to_lowercase: LettersToLowercaseReplacement
    punct_unshifted_replacement: KeysetOneToOneReplacement
    # one_to_many: KeysetOneToManyReplacement


@dataclass_json
@dataclass
class KeysetConfig:
    keyset: str
    replacement: KeysetReplacementConfig
