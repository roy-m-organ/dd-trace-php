<?php

return [
    __DIR__ . '/configuration.php',
    __DIR__ . '/../src/private/functions.php',
    __DIR__ . '/../src/api/Contracts/Tracer.php',
    __DIR__ . '/../src/api/Contracts/Span.php',
    __DIR__ . '/../src/api/Contracts/Scope.php',
    __DIR__ . '/../src/api/Contracts/ScopeManager.php',
    __DIR__ . '/../src/api/Contracts/SpanContext.php',
    __DIR__ . '/../src/api/Log/LoggingTrait.php',
    __DIR__ . '/../src/api/Data/SpanContext.php',
    __DIR__ . '/../src/api/Data/Span.php',
    __DIR__ . '/../src/DDTrace/Sampling/Sampler.php',
    __DIR__ . '/../src/api/Transport.php',
    __DIR__ . '/../src/DDTrace/SpanContext.php',
    __DIR__ . '/../src/DDTrace/Span.php',
    __DIR__ . '/../src/DDTrace/Tracer.php',
    __DIR__ . '/../src/api/Obfuscation/WildcardToRegex.php',
    __DIR__ . '/../src/DDTrace/StartSpanOptionsFactory.php',
    __DIR__ . '/../src/DDTrace/Time.php',
    __DIR__ . '/../src/DDTrace/Transport/Http.php',
    __DIR__ . '/../src/api/Type.php',
    __DIR__ . '/../src/DDTrace/Encoder.php',
    __DIR__ . '/../src/DDTrace/Util/Runtime.php',
    __DIR__ . '/../src/DDTrace/Util/Versions.php',
    __DIR__ . '/../src/DDTrace/Util/ObjectKVStore.php',
    __DIR__ . '/../src/DDTrace/Util/ArrayKVStore.php',
    __DIR__ . '/../src/DDTrace/Processing/TraceAnalyticsProcessor.php',
    __DIR__ . '/../src/api/Tag.php',
    __DIR__ . '/../src/DDTrace/Scope.php',
    __DIR__ . '/../src/api/Reference.php',
    __DIR__ . '/../src/DDTrace/Sampling/AlwaysKeepSampler.php',
    __DIR__ . '/../src/DDTrace/Sampling/PrioritySampling.php',
    __DIR__ . '/../src/DDTrace/Sampling/ConfigurableSampler.php',
    __DIR__ . '/../src/DDTrace/Propagator.php',
    __DIR__ . '/../src/DDTrace/Bootstrap.php',
    __DIR__ . '/../src/DDTrace/Encoders/SpanEncoder.php',
    __DIR__ . '/../src/DDTrace/Encoders/MessagePack.php',
    __DIR__ . '/../src/api/Exceptions/InvalidReferenceArgument.php',
    __DIR__ . '/../src/api/Exceptions/UnsupportedFormat.php',
    __DIR__ . '/../src/api/Exceptions/InvalidSpanArgument.php',
    __DIR__ . '/../src/api/Exceptions/InvalidReferencesSet.php',
    __DIR__ . '/../src/api/Exceptions/InvalidSpanOption.php',

    __DIR__ . '/../src/api/GlobalTracer.php',
    __DIR__ . '/../src/DDTrace/Propagators/TextMap.php',
    __DIR__ . '/../src/DDTrace/Propagators/CurlHeadersMap.php',
    __DIR__ . '/../src/api/Http/Urls.php',
    __DIR__ . '/../src/DDTrace/Http/Request.php',
    __DIR__ . '/../src/DDTrace/ScopeManager.php',

    __DIR__ . '/../src/DDTrace/Integrations/AbstractIntegrationConfiguration.php',
    __DIR__ . '/../src/DDTrace/Integrations/DefaultIntegrationConfiguration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Integration.php',
    __DIR__ . '/../src/DDTrace/Integrations/CakePHP/CakePHPIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/CodeIgniter/V2/CodeIgniterIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Web/WebIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/IntegrationsLoader.php',
    __DIR__ . '/../src/DDTrace/Integrations/PDO/PDOIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/PHPRedis/PHPRedisIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Predis/PredisIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Eloquent/EloquentIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Memcached/MemcachedIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Curl/CurlIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Mysqli/MysqliCommon.php',
    __DIR__ . '/../src/DDTrace/Integrations/Mysqli/MysqliIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Mongo/MongoIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Slim/SlimIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Symfony/SymfonyIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/ElasticSearch/V1/ElasticSearchCommon.php',
    __DIR__ . '/../src/DDTrace/Integrations/ElasticSearch/V1/ElasticSearchIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Laravel/LaravelIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Lumen/LumenIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Guzzle/GuzzleIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Yii/YiiIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/Nette/NetteIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/WordPress/WordPressIntegration.php',
    __DIR__ . '/../src/DDTrace/Integrations/WordPress/V4/WordPressIntegrationLoader.php',
    __DIR__ . '/../src/DDTrace/Integrations/ZendFramework/ZendFrameworkIntegration.php',

    __DIR__ . '/../src/DDTrace/Log/Logger.php',
    __DIR__ . '/../src/api/Log/LoggerInterface.php',
    __DIR__ . '/../src/api/Log/InterpolateTrait.php',
    __DIR__ . '/../src/api/Log/LogLevel.php',
    __DIR__ . '/../src/api/Log/AbstractLogger.php',
    __DIR__ . '/../src/api/Log/ErrorLogLogger.php',
    __DIR__ . '/../src/api/Log/NullLogger.php',

    __DIR__ . '/../src/DDTrace/Obfuscation.php',
    __DIR__ . '/../src/DDTrace/Format.php',
    __DIR__ . '/../src/api/StartSpanOptions.php',
];
