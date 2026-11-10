<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields\Constraints {
    #[\Attribute]
    final readonly class MultipleOf implements Field {
        public int|float $value;

        public function __construct(int|float $value) {}
    }
}
