<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Exceptions {
    interface StrictifyThrowable extends \Throwable {}

    class ValidationException extends \Exception implements StrictifyThrowable {
       public function __construct(array $allErrors) {}
       public function getErrors(): array {}
    }

    class InvalidBaseModelException extends \Error implements StrictifyThrowable {}

    class MaxDepthException extends \Error implements StrictifyThrowable {}

    class CallException extends \ArgumentCountError implements StrictifyThrowable {}
}
