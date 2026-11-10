<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields\Constraints {
    #[\Attribute]
    final readonly class In implements Field {
        public array $values;

        public function __construct(string|int|float|bool ...$values) {}
    }
}
