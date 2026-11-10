<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields {
    #[\Attribute]
    readonly class Dict implements Field {
        public int|\Attributes\Strictify\Type|Union $key;
        public int|string|\Attributes\Strictify\Type|Union|Intersection|Sequence|Dict $of;
        public bool $uniqueValues;

        public function __construct(int|\Attributes\Strictify\Type|Union $key, int|string|\Attributes\Strictify\Type|Union|Intersection|Sequence|Dict $of, bool $uniqueValues = false) {}
    }
}
