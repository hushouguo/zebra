DROP DATABASE IF EXISTS `zebra`;
CREATE DATABASE `zebra` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;

USE `zebra`

--
-- `acct`
--
DROP TABLE IF EXISTS `acct`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `acct` (
    `id` bigint unsigned NOT NULL,
    `data` longblob NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='acct table';
SET character_set_client = @saved_cs_client;

--
-- `user`
--
DROP TABLE IF EXISTS `user`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user` (
    `id` bigint unsigned NOT NULL,
    `data` longblob NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='user table';
SET character_set_client = @saved_cs_client;

--
-- `order`
--
DROP TABLE IF EXISTS `order`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `order` (
    `id` bigint unsigned NOT NULL,
    `data` longblob NOT NULL,
	INDEX (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='order table';
SET character_set_client = @saved_cs_client;



