FROM php:7.3-cli

RUN apt-get update \
  && apt-get install -y git unzip \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /var/www

COPY --from=composer:2 /usr/bin/composer /usr/local/bin/composer

COPY src /var/www

EXPOSE 80

RUN composer install

CMD [ "php", "-S", "0.0.0.0:80", "index.php" ]
