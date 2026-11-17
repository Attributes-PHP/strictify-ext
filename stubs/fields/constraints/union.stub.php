<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields\Constraints {
    #[\Attribute]
    final readonly class Union implements Field {
        public array $of;

        public function __construct(int|string|\Attributes\Strictify\Type ...$of) {}
    }
}
