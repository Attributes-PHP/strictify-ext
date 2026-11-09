<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class Size implements Field {
        public ?int $min;
        public ?int $max;

        public function __construct(?int $min = null, ?int $max = null) {}
    }
}
