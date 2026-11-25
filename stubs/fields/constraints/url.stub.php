<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields\Constraints {
    #[\Attribute]
    final readonly class Url implements Field {
        /** Allowed schemes (the part before ://): a string or a list of strings; null allows any */
        public string|array|null $protocol;

        /** Allowed hosts: a string or a list of strings; null allows any */
        public string|array|null $domain;

        public function __construct(string|array|null $protocol = null, string|array|null $domain = null) {}
    }
}
