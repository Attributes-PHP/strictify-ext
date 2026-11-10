<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchDateTimeEvent extends BaseModel
{
    public \DateTime $createdAt;

    public \DateTime $updatedAt;

    public \DateTime $deletedAt;

    public \DateTime $archivedAt;
}
