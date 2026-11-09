<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
    #[Attribute]
    class Dict implements Field {
        public int|\Attributes\Validation\Type|Union $key;
        public int|string|\Attributes\Validation\Type|Union|Intersection|Sequence|Dict $of;
        public bool $uniqueValues;

        public function __construct(int|\Attributes\Validation\Type|Union $key, int|string|\Attributes\Validation\Type|Union|Intersection|Sequence|Dict $of, bool $uniqueValues = false) {}
    }
}
