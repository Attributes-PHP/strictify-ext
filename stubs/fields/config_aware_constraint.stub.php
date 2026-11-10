<?php
/** @generate-class-entries */

namespace Attributes\Strictify\Fields {
   interface ConfigAwareConstraint extends Constraint {
      public function setModelConfigs(\Attributes\Strictify\ModelConfigs $configs): void;
   }
}
