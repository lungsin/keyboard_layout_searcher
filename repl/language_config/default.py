from repl.keyset_config import (
    KeysetConfig,
    KeysetOneToOneReplacement,
    KeysetReplacementConfig,
    LettersToLowercaseReplacement,
)

DefaultKeysetConfig = KeysetConfig(
    keyset="abcdefghijklmnopqrstuvwxyz',.;",
    replacement=KeysetReplacementConfig(
        one_to_one=KeysetOneToOneReplacement(
            before="´÷‘’–ʹ͵",
            after="'/''-''",
        ),
        letters_to_lowercase=LettersToLowercaseReplacement(
            list="abcdefghijklmnopqrstuvwxyz",
        ),
        punct_unshifted_replacement=KeysetOneToOneReplacement(
            before='\{\}?+_|"<>:~“”«»',
            after="[]/=-\\',.;`''''",
        ),
    ),
)
