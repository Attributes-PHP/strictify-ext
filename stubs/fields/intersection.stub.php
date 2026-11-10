<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields {
    #[\Attribute]
    readonly class Intersection implements Field {
        public array $of;

        public function __construct(string ...$of) {}
    }
}
