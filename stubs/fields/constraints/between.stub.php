<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields\Constraints {
    #[\Attribute]
    final readonly class Between implements Field {
        public int|float $min;
        public int|float $max;

        public function __construct(int|float $min, int|float $max) {}
    }
}
