<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
    #[\Attribute]
    readonly class Union implements Field {
        public array $of;

        public function __construct(int|string|\Attributes\Validation\Type|Sequence|Union|Intersection|Dict ...$of) {}
    }
}
