<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields {
    #[\Attribute]
    readonly class Sequence implements Field {
        public int|string|\Attributes\Strictify\Type|Sequence|Union|Intersection|Dict $of;
        public bool $uniqueValues;
        public bool $sequential;
        /** 'asc', 'ASC', 'desc', 'DESC' to sort the validated values, or null to keep the input order */
        public ?string $sorted;

        public function __construct(int|string|\Attributes\Strictify\Type|Sequence|Union|Intersection|Dict $of, bool $uniqueValues = false, bool $sequential = true, ?string $sorted = null) {}
    }
}
