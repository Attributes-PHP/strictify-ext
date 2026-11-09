<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
   #[\Attribute]
   final class DateTime implements Field {
      public readonly string $format;
      public readonly \DateTimeZone|string|null $timezone;

      /**
       * @internal
       */
      private \DateTimeZone|string|null $timezone_obj;

      public function __construct(string $format = 'X-m-d\\TH:i:sP', \DateTimeZone|string|null $timezone = null) {}
   }
}