<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
    #[\Attribute]
    readonly class Sequence implements Field {
        public int|string|\Attributes\Validation\Type|Sequence|Union|Intersection|Dict $of;
        public bool $uniqueValues;
        public bool $sequential;

        public function __construct(int|string|\Attributes\Validation\Type|Sequence|Union|Intersection|Dict $of, bool $uniqueValues = false, bool $sequential = true) {}
    }
}
