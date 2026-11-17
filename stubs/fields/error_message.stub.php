<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields {
   #[\Attribute]
   readonly class ErrorMessage implements Field {
      public string $required = 'Field is required';
      public string $type = 'Must be {expected}';
      public string $alpha_dash = 'Must only contain letters, numbers, dashes and underscores';
      public string $alpha_dash_type = 'Must be a string containing only letters, numbers, dashes and underscores';
      public string $ascii = 'Must only contain ASCII characters';
      public string $ascii_type = 'Must be a string containing only ASCII characters';
      public string $between = 'Must be between {expected}';
      public string $between_type = 'Must be a number between {expected}';
      public string $contains = 'Must contain {expected}';
      public string $datetime = 'Must be a datetime in the format {expected}';
      public string $ends_with = 'Must end with one of: {expected}';
      public string $in = 'Must be one of: {expected}';
      public string $max = 'Must be at most {expected}';
      public string $min = 'Must be at least {expected}';
      public string $multiple_of = 'Must be a multiple of {expected}';
      public string $regex = 'Must match the pattern {expected}';
      public string $size = 'Must have a size {expected}';
      public string $starts_with = 'Must start with one of: {expected}';
      public string $ulid = 'Must be a valid ULID';
      public string $uuid = 'Must be a valid UUID';
      public string $union = 'Must be {expected}';
      public string $sequence_keys = 'Must be a sequential list (keys 0, 1, 2, ...)';

      public function __construct(
         string $required = 'Field is required',
         string $type = 'Must be {expected}',
         string $alpha_dash = 'Must only contain letters, numbers, dashes and underscores',
         string $alpha_dash_type = 'Must be a string containing only letters, numbers, dashes and underscores',
         string $ascii = 'Must only contain ASCII characters',
         string $ascii_type = 'Must be a string containing only ASCII characters',
         string $between = 'Must be between {expected}',
         string $between_type = 'Must be a number between {expected}',
         string $contains = 'Must contain {expected}',
         string $datetime = 'Must be a datetime in the format {expected}',
         string $ends_with = 'Must end with one of: {expected}',
         string $in = 'Must be one of: {expected}',
         string $max = 'Must be at most {expected}',
         string $min = 'Must be at least {expected}',
         string $multiple_of = 'Must be a multiple of {expected}',
         string $regex = 'Must match the pattern {expected}',
         string $size = 'Must have a size {expected}',
         string $starts_with = 'Must start with one of: {expected}',
         string $ulid = 'Must be a valid ULID',
         string $uuid = 'Must be a valid UUID',
         string $union = 'Must be {expected}',
         string $sequence_keys = 'Must be a sequential list (keys 0, 1, 2, ...)',
      ) {}
   }
}
