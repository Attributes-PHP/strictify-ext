<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields\Constraints {
    #[\Attribute]
    final readonly class ContainsAny implements Field {
        public array $values;

        public function __construct(string|int|float ...$values) {}
    }
}
