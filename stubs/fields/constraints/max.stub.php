<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields\Constraints {
    #[\Attribute]
    final readonly class Max implements Field {
        public int|float $value;

        public function __construct(int|float $value) {}
    }
}
