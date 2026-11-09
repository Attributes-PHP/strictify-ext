<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class Between implements Field {
        public int|float $min;
        public int|float $max;

        public function __construct(int|float $min, int|float $max) {}
    }
}
