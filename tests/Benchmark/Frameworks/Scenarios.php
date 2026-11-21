<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Frameworks;

/**
 * Input data shared by the framework comparison benches, mirroring the
 * strictify benchmark scenarios field for field.
 */
final class Scenarios
{
    public static function simple(): array
    {
        return [
            'age' => 30,
            'name' => 'Andre',
            'email' => 'andre@example.com',
            'active' => true,
            'balance' => 1250.50,
        ];
    }

    /**
     * Stringly-typed scalars: frameworks that coerce accept them as-is,
     * Symfony is given relaxed union types instead (it never coerces).
     */
    public static function loose(): array
    {
        return [
            'age' => '30',
            'name' => 'Andre',
            'email' => 'andre@example.com',
            'active' => 1,
            'balance' => '1250.50',
        ];
    }

    public static function constrained(bool $fail = false): array
    {
        $data = [
            'slug' => 'user-1_name',
            'label' => 'hello',
            'age' => 30,
            'tags' => ['php', 'dev', 'oss'],
            'birthday' => '1994-01-01',
            'email' => 'andre@example.com',
            'grade' => 'b',
            'score' => 99.5,
            'quantity' => 1,
            'packSize' => 20,
            'username' => 'user-name',
            'name' => 'andre',
            'website' => 'https://example.com',
            'eventId' => '01ARZ3NDEKTSV4RRFFQ69G5FAV',
            'requestId' => '123e4567-e89b-12d3-a456-426614174000',
        ];

        if ($fail) {
            $data['age'] = 17; // Between(18, 99) fails
        }

        return $data;
    }

    public static function nested(): array
    {
        $addresses = [];
        $cities = ['Porto', 'Lisbon', 'Braga', 'Faro', 'Aveiro', 'Coimbra', 'Setubal', 'Guimaraes', 'Viseu', 'Evora'];
        foreach ($cities as $i => $city) {
            $addresses[] = ['street' => 'Street ' . $i, 'city' => $city];
        }

        return ['id' => 42, 'addresses' => $addresses];
    }

    public static function wide(): array
    {
        $data = [];
        for ($i = 1; $i <= 32; $i++) {
            $name = 'field' . str_pad((string) $i, 2, '0', STR_PAD_LEFT);
            $data[$name] = match (($i - 1) % 8) {
                0, 1 => 'value-' . $i,
                2, 3 => $i,
                4, 5 => $i + 0.5,
                6, 7 => $i % 2 === 1,
            };
        }

        return $data;
    }

    public static function matrix(): array
    {
        $data = ['matrix' => [], 'groups' => ['seed' => [1]]];
        for ($i = 0; $i < 20; $i++) {
            $data['matrix'][] = range($i * 50, $i * 50 + 49);
        }

        return $data;
    }

    public static function dictOfSequences(): array
    {
        $data = ['matrix' => [[1]], 'groups' => []];
        for ($i = 0; $i < 20; $i++) {
            $data['groups']['group-' . $i] = range(0, 49);
        }

        return $data;
    }

    public static function unionScalars(): array
    {
        $mixed = [];
        for ($i = 0; $i < 100; $i++) {
            $mixed[] = match ($i % 3) {
                0 => $i,
                1 => 'item-' . $i,
                2 => ($i % 2) === 0,
            };
        }

        return ['mixed' => $mixed, 'coerced' => []];
    }

    /**
     * @return array<string, array<string, mixed>>
     */
    public static function nineModels(): array
    {
        return [
            'profile' => ['username' => 'andre', 'email' => 'andre@example.com', 'loginCount' => 42],
            'signup' => ['user_name' => 'andre', 'email' => 'andre@example.com', 'age' => '30'],
            'product' => ['sku' => 'SKU-1', 'title' => 'Validation extension', 'price' => 99.9, 'stock' => 10],
            'session' => ['sessionId' => 'abc123', 'userId' => 'user-1', 'rememberMe' => true],
            'cart' => ['sku' => 'SKU-1', 'quantity' => 2, 'unitPrice' => 49.95],
            'invoice' => ['number' => 'INV-2026-001', 'customer' => 'Acme Inc', 'total' => 1250.50],
            'review' => ['author' => 'andre', 'comment' => 'Fast validation', 'rating' => 5],
            'category' => ['slug' => 'php-extensions', 'name' => 'PHP Extensions'],
            'tags' => ['name' => 'validation', 'tags' => ['php', 'c', 'extension']],
        ];
    }
}
