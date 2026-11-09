<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
   #[\Attribute]
   readonly class ErrorMessage implements Field {
      public string $required;
      public string $type;

      public function __construct(string $required, string $type) {}
   }
}