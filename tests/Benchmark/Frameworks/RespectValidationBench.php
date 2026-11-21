<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Frameworks;

use Attributes\Strictify\Tests\Benchmark\Scenarios;

use PhpBench\Attributes as Bench;
use Respect\Validation\Validator;

/**
 * The strictify benchmark scenarios expressed with respect/validation.
 * Rule objects are built once in the constructor; subjects call
 * validate() (returns bool, no message building) per rev.
 */
#[Bench\OutputTimeUnit('microseconds')]
class RespectValidationBench
{
    /** @var array<string, Validator> */
    private array $rules = [];

    public function __construct()
    {
        $this->rules['simple'] = Validator::key('age', Validator::intVal())
            ->key('name', Validator::stringType())
            ->key('email', Validator::stringType())
            ->key('active', Validator::boolVal())
            ->key('balance', Validator::floatVal());

        // intVal/floatVal/boolVal accept numeric scalar strings
        $this->rules['loose'] = $this->rules['simple'];

        $this->rules['constrained'] = Validator::key('slug', Validator::regex('/^[a-zA-Z0-9_-]+$/'))
            ->key('label', Validator::regex('/^[\x00-\x7F]*$/'))
            ->key('age', Validator::intVal()->between(18, 99))
            ->key('tags', Validator::arrayVal()->contains('php')->contains('dev'))
            ->key('birthday', Validator::date('Y-m-d'))
            ->key('email', Validator::stringType()->callback(static fn ($v): bool => str_ends_with($v, '.com') || str_ends_with($v, '.net')))
            ->key('grade', Validator::in(['a', 'b', 'c']))
            ->key('score', Validator::floatVal()->max(100))
            ->key('quantity', Validator::intVal()->min(1))
            ->key('packSize', Validator::intVal()->callback(static fn ($v): bool => $v % 5 === 0))
            ->key('username', Validator::regex('/^[a-z][a-z0-9-]*$/'))
            ->key('name', Validator::stringType()->length(3, 20))
            ->key('website', Validator::stringType()->startsWith('https://'))
            ->key('eventId', Validator::regex('/^[0-7][0-9A-HJKMNP-TV-Z]{25}$/i'))
            ->key('requestId', Validator::uuid());

        $this->rules['nested'] = Validator::key('id', Validator::intVal())
            ->key('addresses', Validator::arrayVal()->each(
                Validator::key('street', Validator::stringType())->key('city', Validator::stringType())
            ));

        $wide = Validator::arrayVal();
        for ($i = 1; $i <= 32; $i++) {
            $name = 'field' . str_pad((string) $i, 2, '0', STR_PAD_LEFT);
            $rule = match (($i - 1) % 8) {
                0, 1 => Validator::stringType(),
                2, 3 => Validator::intVal(),
                4, 5 => Validator::floatVal(),
                6, 7 => Validator::boolVal(),
            };
            $wide = $wide->key($name, $rule);
        }
        $this->rules['wide'] = $wide;

        $intLists = Validator::arrayVal()->each(Validator::arrayVal()->each(Validator::intVal()));
        $this->rules['matrix'] = Validator::key('matrix', $intLists)->key('groups', $intLists);
        $this->rules['dict'] = $this->rules['matrix'];

        $this->rules['union'] = Validator::key('mixed', Validator::arrayVal()->each(
            Validator::oneOf(Validator::intVal(), Validator::stringType(), Validator::boolVal())
        ))->key('coerced', Validator::arrayVal()->each(
            Validator::oneOf(Validator::intVal(), Validator::floatVal())
        ));

        $this->rules['nine'] = [];
        $nineRules = [
            'profile' => Validator::key('username', Validator::stringType())->key('email', Validator::stringType())->key('loginCount', Validator::intVal()),
            'signup' => Validator::key('user_name', Validator::stringType())->key('email', Validator::stringType())->key('age', Validator::intVal()),
            'product' => Validator::key('sku', Validator::stringType())->key('title', Validator::stringType())->key('price', Validator::floatVal())->key('stock', Validator::intVal()),
            'session' => Validator::key('sessionId', Validator::stringType())->key('userId', Validator::stringType())->key('rememberMe', Validator::boolVal()),
            'cart' => Validator::key('sku', Validator::stringType())->key('quantity', Validator::intVal())->key('unitPrice', Validator::floatVal()),
            'invoice' => Validator::key('number', Validator::stringType())->key('customer', Validator::stringType())->key('total', Validator::floatVal()),
            'review' => Validator::key('author', Validator::stringType())->key('comment', Validator::stringType())->key('rating', Validator::intVal()),
            'category' => Validator::key('slug', Validator::stringType())->key('name', Validator::stringType()),
            'tags' => Validator::key('name', Validator::stringType())->key('tags', Validator::arrayVal()->each(Validator::stringType())),
        ];
        $this->rules['nine'] = $nineRules;

        // Warm the rules so subjects measure steady-state cost
        foreach ($this->rules as $key => $rule) {
            if ($key === 'nine') {
                foreach ($rule as $name => $r) {
                    $r->validate(Scenarios::nineModels()[$name]);
                }
            } else {
                $rule->validate([]);
            }
        }
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateSimpleModel(): void
    {
        $this->rules['simple']->validate(Scenarios::simple());
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateLooseModel(): void
    {
        $this->rules['loose']->validate(Scenarios::loose());
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateConstrainedModel(): void
    {
        $this->rules['constrained']->validate(Scenarios::constrained());
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateConstrainedModelFailing(): void
    {
        $this->rules['constrained']->validate(Scenarios::constrained(true));
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNestedModel(): void
    {
        $this->rules['nested']->validate(Scenarios::nested());
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidate32FieldModel(): void
    {
        $this->rules['wide']->validate(Scenarios::wide());
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateNestedSequences(): void
    {
        $this->rules['matrix']->validate(Scenarios::matrix());
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateDictOfSequences(): void
    {
        $this->rules['dict']->validate(Scenarios::dictOfSequences());
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateUnionExactScalars(): void
    {
        $this->rules['union']->validate(Scenarios::unionScalars());
    }

    #[Bench\Subject, Bench\Groups(['frameworks'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNineDistinctModels(): void
    {
        foreach (Scenarios::nineModels() as $name => $data) {
            $this->rules['nine'][$name]->validate($data);
        }
    }
}
