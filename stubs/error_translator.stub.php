<?php
/** @generate-class-entries */

namespace Attributes\Strictify {
   final class ErrorTranslator {
      private static ?callable $translator = null;

      public static function get(): ?callable {}
      public static function set(callable $translator): void {}
   }
}