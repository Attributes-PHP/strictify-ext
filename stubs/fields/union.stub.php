<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields {
    #[\Attribute]
    readonly class Union implements Field {
        public array $of;

        public function __construct(int|string|\Attributes\Strictify\Type|Sequence|Union|Intersection|Dict ...$of) {}
    }
}
