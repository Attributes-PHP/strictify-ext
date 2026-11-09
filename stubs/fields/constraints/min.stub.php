<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class Min implements Field {
        public int|float $value;

        public function __construct(int|float $value) {}
    }
}
