<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Frameworks;

use Attributes\Strictify\Tests\Benchmark\Scenarios;

use Illuminate\Translation\ArrayLoader;
use Illuminate\Translation\Translator;
use Illuminate\Validation\Factory;
use Illuminate\Validation\ValidationException;
use PhpBench\Attributes as Bench;

/**
 * The strictify benchmark scenarios expressed with illuminate/validation
 * (the Laravel validator). The Factory is built once; subjects call
 * make()->validate() per rev, which throws on failure like strictify.
 */
#[Bench\OutputTimeUnit('microseconds')]
class LaravelValidationBench
{
    private Factory $factory;

    /** @var array<string, array<string, mixed>> */
    private array $rules = [];

    public function __construct()
    {
        $this->factory = new Factory(new Translator(new ArrayLoader(), 'en'));

        $simple = [
            'age' => 'integer',
            'name' => 'string',
            'email' => 'string',
            'active' => 'boolean',
            'balance' => 'numeric',
        ];
        $this->rules['simple'] = $simple;
        // Laravel's integer/boolean/numeric rules accept the stringly
        // scalars of the loose scenario as-is
        $this->rules['loose'] = $simple;

        $this->rules['constrained'] = [
            'slug' => 'regex:/^[a-zA-Z0-9_-]+$/',
            'label' => 'regex:/^[\x00-\x7F]*$/',
            'age' => ['integer', 'between:18,99'],
            'tags' => ['array', static function ($attribute, $value, $fail): void {
                foreach (['php', 'dev'] as $needed) {
                    if (!in_array($needed, (array) $value, true)) {
                        $fail('The '.$attribute.' must contain '.$needed.'.');
                    }
                }
            }],
            'birthday' => 'date_format:Y-m-d',
            'email' => ['string', 'ends_with:.com,.net'],
            'grade' => 'in:a,b,c',
            'score' => ['numeric', 'max:100'],
            'quantity' => ['integer', 'min:1'],
            'packSize' => ['integer', static function ($attribute, $value, $fail): void {
                if ($value % 5 !== 0) {
                    $fail('The '.$attribute.' must be a multiple of 5.');
                }
            }],
            'username' => 'regex:/^[a-z][a-z0-9-]*$/',
            'name' => ['string', 'between:3,20'],
            'website' => ['string', 'starts_with:https://'],
            'eventId' => 'regex:/^[0-7][0-9A-HJKMNP-TV-Z]{25}$/i',
            'requestId' => 'uuid',
        ];

        $this->rules['nested'] = [
            'id' => 'integer',
            'addresses' => 'array',
            'addresses.*.street' => 'string',
            'addresses.*.city' => 'string',
        ];

        $wide = [];
        for ($i = 1; $i <= 32; $i++) {
            $name = 'field' . str_pad((string) $i, 2, '0', STR_PAD_LEFT);
            $wide[$name] = match (($i - 1) % 8) {
                0, 1 => 'string',
                2, 3 => 'integer',
                4, 5 => 'numeric',
                6, 7 => 'boolean',
            };
        }
        $this->rules['wide'] = $wide;

        $shape = [
            'matrix' => 'array',
            'matrix.*' => 'array',
            'matrix.*.*' => 'integer',
            'groups' => 'array',
            'groups.*' => 'array',
            'groups.*.*' => 'integer',
        ];
        $this->rules['matrix'] = $shape;
        $this->rules['dict'] = $shape;

        $this->rules['union'] = [
            'mixed' => 'array',
            'mixed.*' => static function ($attribute, $value, $fail): void {
                if (!is_int($value) && !is_string($value) && !is_bool($value)) {
                    $fail('The '.$attribute.' must be an integer, string or boolean.');
                }
            },
            'coerced' => 'array',
            'coerced.*' => static function ($attribute, $value, $fail): void {
                if (!is_int($value) && !is_float($value)) {
                    $fail('The '.$attribute.' must be an integer or float.');
                }
            },
        ];

        $this->rules['nine'] = [
            'profile' => ['username' => 'string', 'email' => 'string', 'loginCount' => 'integer'],
            'signup' => ['user_name' => 'string', 'email' => 'string', 'age' => 'integer'],
            'product' => ['sku' => 'string', 'title' => 'string', 'price' => 'numeric', 'stock' => 'integer'],
            'session' => ['sessionId' => 'string', 'userId' => 'string', 'rememberMe' => 'boolean'],
            'cart' => ['sku' => 'string', 'quantity' => 'integer', 'unitPrice' => 'numeric'],
            'invoice' => ['number' => 'string', 'customer' => 'string', 'total' => 'numeric'],
            'review' => ['author' => 'string', 'comment' => 'string', 'rating' => 'integer'],
            'category' => ['slug' => 'string', 'name' => 'string'],
            'tags' => ['name' => 'string', 'tags' => 'array', 'tags.*' => 'string'],
        ];

        // Warm the rule/message caches so subjects measure steady-state cost
        foreach ($this->rules as $key => $rules) {
            if ($key === 'nine') {
                foreach (Scenarios::nineModels() as $name => $data) {
                    $this->factory->make($data, $rules[$name])->validate();
                }
            } else {
                $this->factory->make([], $rules)->passes();
            }
        }
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateSimpleModel(): void
    {
        $this->factory->make(Scenarios::simple(), $this->rules['simple'])->validate();
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateLooseModel(): void
    {
        $this->factory->make(Scenarios::loose(), $this->rules['loose'])->validate();
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateConstrainedModel(): void
    {
        $this->factory->make(Scenarios::constrained(), $this->rules['constrained'])->validate();
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateConstrainedModelFailing(): void
    {
        try {
            $this->factory->make(Scenarios::constrained(true), $this->rules['constrained'])->validate();
        } catch (ValidationException) {
            // The expected outcome: one violation, message built
        }
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNestedModel(): void
    {
        $this->factory->make(Scenarios::nested(), $this->rules['nested'])->validate();
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidate32FieldModel(): void
    {
        $this->factory->make(Scenarios::wide(), $this->rules['wide'])->validate();
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateNestedSequences(): void
    {
        $this->factory->make(Scenarios::matrix(), $this->rules['matrix'])->validate();
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateDictOfSequences(): void
    {
        $this->factory->make(Scenarios::dictOfSequences(), $this->rules['dict'])->validate();
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateUnionExactScalars(): void
    {
        $this->factory->make(Scenarios::unionScalars(), $this->rules['union'])->validate();
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNineDistinctModels(): void
    {
        foreach (Scenarios::nineModels() as $name => $data) {
            $this->factory->make($data, $this->rules['nine'][$name])->validate();
        }
    }
}
