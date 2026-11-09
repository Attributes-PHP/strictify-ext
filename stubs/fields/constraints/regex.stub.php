<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class Regex implements Field {
        public string $regex;

        public function __construct(string $regex) {}
    }
}
