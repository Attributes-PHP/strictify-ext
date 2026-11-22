<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark;

use Attributes\Strictify\Tests\Benchmark\Models\BenchCartItem;
use Attributes\Strictify\Tests\Benchmark\Models\BenchCategory;
use Attributes\Strictify\Tests\Benchmark\Models\BenchInvoice;
use Attributes\Strictify\Tests\Benchmark\Models\BenchProduct;
use Attributes\Strictify\Tests\Benchmark\Models\BenchProfile;
use Attributes\Strictify\Tests\Benchmark\Models\BenchReview;
use Attributes\Strictify\Tests\Benchmark\Models\BenchSession;
use Attributes\Strictify\Tests\Benchmark\Models\BenchSignup;
use Attributes\Strictify\Tests\Benchmark\Models\BenchTagList;
use PhpBench\Attributes as Bench;

use function Attributes\Strictify\validate;

#[Bench\OutputTimeUnit('microseconds')]
class MultipleModelsValidationBench
{
    private array $data;

    private array $models;

    public function __construct()
    {
        $classes = [
            'profile' => BenchProfile::class,
            'signup' => BenchSignup::class,
            'product' => BenchProduct::class,
            'session' => BenchSession::class,
            'cart' => BenchCartItem::class,
            'invoice' => BenchInvoice::class,
            'review' => BenchReview::class,
            'category' => BenchCategory::class,
            'tags' => BenchTagList::class,
        ];
        $this->data = [];
        foreach (Scenarios::nineModels() as $name => $data) {
            $class = $classes[$name];
            $this->data[$class] = $data;
        }

        $this->models = [];
        foreach (array_keys($this->data) as $class) {
            $this->models[] = new $class;
        }
    }

    #[Bench\Subject, Bench\Groups(['multiple-models'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNineDistinctModels(): void
    {
        foreach ($this->models as $model) {
            validate($this->data[$model::class], $model);
        }
    }

    #[Bench\Subject, Bench\Groups(['multiple-models'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateSameModelTenTimes(): void
    {
        $model = $this->models[0];
        $data = $this->data[$model::class];

        for ($i = 0; $i < 10; $i++) {
            validate($data, $model);
        }
    }
}
