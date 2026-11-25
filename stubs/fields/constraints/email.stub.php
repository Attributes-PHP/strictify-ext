<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields\Constraints {
    #[\Attribute]
    final readonly class Email implements Field {
        /** Allowed domains after the @: a string or a list of strings; null allows any */
        public string|array|null $domain;

        public function __construct(string|array|null $domain = null) {}
    }
}
