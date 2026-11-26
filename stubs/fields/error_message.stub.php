<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields {
   #[\Attribute]
   readonly class ErrorMessage implements Field {
      public string $required = 'Field is required';
      public string $type = 'Must be {expected}';
      public string $alphaDash = 'Must only contain letters, numbers, dashes and underscores';
      public string $alphaDashType = 'Must be a string containing only letters, numbers, dashes and underscores';
      public string $ascii = 'Must only contain ASCII characters';
      public string $asciiType = 'Must be a string containing only ASCII characters';
      public string $between = 'Must be between {expected}';
      public string $betweenType = 'Must be a number between {expected}';
      public string $contains = 'Must contain {expected}';
      public string $datetime = 'Must be a datetime in the format {expected}';
      public string $endsWith = 'Must end with one of: {expected}';
      public string $in = 'Must be one of: {expected}';
      public string $max = 'Must be at most {expected}';
      public string $min = 'Must be at least {expected}';
      public string $multipleOf = 'Must be a multiple of {expected}';
      public string $regex = 'Must match the pattern {expected}';
      public string $size = 'Must have a size {expected}';
      public string $startsWith = 'Must start with one of: {expected}';
      public string $ulid = 'Must be a valid ULID';
      public string $uuid = 'Must be a valid UUID';
      public string $email = 'Must be a valid email address';
      public string $emailDomain = 'Must be an email address on an allowed domain: {expected}';
      public string $url = 'Must be a valid URL';
      public string $urlProtocol = 'Must be a URL using an allowed protocol: {expected}';
      public string $urlDomain = 'Must be a URL on an allowed domain: {expected}';
      public string $domain = 'Must be a valid domain name';
      public string $ip = 'Must be a valid IP address';
      public string $iban = 'Must be a valid IBAN';
      public string $isbn = 'Must be a valid ISBN';
      public string $creditCard = 'Must be a valid credit card number';
      public string $version = 'Must be a valid version string';
      public string $macAddress = 'Must be a valid MAC address';
      public string $alnum = 'Must contain only letters and numbers';
      public string $alpha = 'Must contain only letters';
      public string $digit = 'Must contain only digits';
      public string $xdigit = 'Must contain only hexadecimal digits';
      public string $slug = 'Must be a valid slug';
      public string $base64 = 'Must be valid base64';
      public string $json = 'Must be valid JSON';
      public string $hexRgbColor = 'Must be a valid hex RGB color';
      public string $even = 'Must be an even number';
      public string $odd = 'Must be an odd number';
      public string $positive = 'Must be a positive number';
      public string $negative = 'Must be a negative number';
      public string $primeNumber = 'Must be a prime number';
      public string $containsAny = 'Must contain at least one of: {expected}';
      public string $union = 'Must be {expected}';
      public string $sequenceKeys = 'Must be a sequential list (keys 0, 1, 2, ...)';
      public string $uniqueValues = 'Must contain only unique values';

      public function __construct(
         string $required = 'Field is required',
         string $type = 'Must be {expected}',
         string $alphaDash = 'Must only contain letters, numbers, dashes and underscores',
         string $alphaDashType = 'Must be a string containing only letters, numbers, dashes and underscores',
         string $ascii = 'Must only contain ASCII characters',
         string $asciiType = 'Must be a string containing only ASCII characters',
         string $between = 'Must be between {expected}',
         string $betweenType = 'Must be a number between {expected}',
         string $contains = 'Must contain {expected}',
         string $datetime = 'Must be a datetime in the format {expected}',
         string $endsWith = 'Must end with one of: {expected}',
         string $in = 'Must be one of: {expected}',
         string $max = 'Must be at most {expected}',
         string $min = 'Must be at least {expected}',
         string $multipleOf = 'Must be a multiple of {expected}',
         string $regex = 'Must match the pattern {expected}',
         string $size = 'Must have a size {expected}',
         string $startsWith = 'Must start with one of: {expected}',
         string $ulid = 'Must be a valid ULID',
         string $uuid = 'Must be a valid UUID',
         string $email = 'Must be a valid email address',
         string $emailDomain = 'Must be an email address on an allowed domain: {expected}',
         string $url = 'Must be a valid URL',
         string $urlProtocol = 'Must be a URL using an allowed protocol: {expected}',
         string $urlDomain = 'Must be a URL on an allowed domain: {expected}',
         string $domain = 'Must be a valid domain name',
         string $ip = 'Must be a valid IP address',
         string $iban = 'Must be a valid IBAN',
         string $isbn = 'Must be a valid ISBN',
         string $creditCard = 'Must be a valid credit card number',
         string $version = 'Must be a valid version string',
         string $macAddress = 'Must be a valid MAC address',
         string $alnum = 'Must contain only letters and numbers',
         string $alpha = 'Must contain only letters',
         string $digit = 'Must contain only digits',
         string $xdigit = 'Must contain only hexadecimal digits',
         string $slug = 'Must be a valid slug',
         string $base64 = 'Must be valid base64',
         string $json = 'Must be valid JSON',
         string $hexRgbColor = 'Must be a valid hex RGB color',
         string $even = 'Must be an even number',
         string $odd = 'Must be an odd number',
         string $positive = 'Must be a positive number',
         string $negative = 'Must be a negative number',
         string $primeNumber = 'Must be a prime number',
         string $containsAny = 'Must contain at least one of: {expected}',
         string $union = 'Must be {expected}',
         string $sequenceKeys = 'Must be a sequential list (keys 0, 1, 2, ...)',
         string $uniqueValues = 'Must contain only unique values',
      ) {}
   }
}
