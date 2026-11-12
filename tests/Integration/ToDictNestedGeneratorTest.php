<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\ModelConfigs;
use Attributes\Strictify\Type;

use function Attributes\Strictify\validate;

it('deeply nested models each use their own aliasGenerator', function () {
    #[ModelConfigs(aliasGenerator: 'camel')]
    class Inner extends BaseModel
    {
        public string $innerValue;
    }

    #[ModelConfigs(aliasGenerator: 'snake')]
    class Middle extends BaseModel
    {
        public Inner $innerModel;
        public string $middleValue;
    }

    $model = new
        #[ModelConfigs(aliasGenerator: 'kebab')]
        class extends BaseModel {
            public Middle $middleModel;
        };

    $input = [
        'middle-model' => [
            'middle_value' => 'mid',
            'inner_model' => ['innerValue' => 'deep'],
        ],
    ];
    validate($input, $model);
    expect($model->middleModel->middleValue)->toBe('mid');
    expect($model->middleModel->innerModel->innerValue)->toBe('deep');

    expect($model->toDict())->toBe([
        'middle-model' => [
            'inner_model' => ['innerValue' => 'deep'],
            'middle_value' => 'mid',
        ],
    ]);
});

it('array of nested models with own aliasGenerator', function () {
    #[ModelConfigs(aliasGenerator: 'camel')]
    class Item extends BaseModel
    {
        public string $itemName;
    }

    $model = new
        #[ModelConfigs(aliasGenerator: 'snake')]
        class extends BaseModel {
            #[Sequence(Item::class)]
            public array $items;
        };

    $input = ['items' => [['itemName' => 'a'], ['itemName' => 'b']]];
    validate($input, $model);
    expect($model->items[0]->itemName)->toBe('a');
    expect($model->toDict())->toBe(['items' => [['itemName' => 'a'], ['itemName' => 'b']]]);
});
