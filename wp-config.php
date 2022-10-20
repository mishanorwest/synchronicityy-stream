<?php
/**
 * The base configurations of the WordPress.
 *
 * This file has the following configurations: MySQL settings, Table Prefix,
 * Secret Keys, WordPress Language, and ABSPATH. You can find more information
 * by visiting {@link http://codex.wordpress.org/Editing_wp-config.php Editing
 * wp-config.php} Codex page. You can get the MySQL settings from your web host.
 *
 * This file is used by the wp-config.php creation script during the
 * installation. You don't have to use the web site, you can just copy this file
 * to "wp-config.php" and fill in the values.
 *
 * @package WordPress
 */

// ** MySQL settings - You can get this info from your web host ** //
/** The name of the database for WordPress */
define('DB_NAME', 'sync23_wordpress1637631944');

/** MySQL database username */
define('DB_USER', 'sync23_2');

/** MySQL database password */
define('DB_PASSWORD', 'AzkWY5LLRGMEZaU9');

/** MySQL hostname */
define('DB_HOST', 'db159.pair.com');

/** Database Charset to use in creating database tables. */
define('DB_CHARSET', 'utf8');

/** The Database Collate type. Don't change this if in doubt. */
define('DB_COLLATE', '');

/**#@+
 * Authentication Unique Keys and Salts.
 *
 * Change these to different unique phrases!
 * You can generate these using the {@link https://api.wordpress.org/secret-key/1.1/salt/ WordPress.org secret-key service}
 * You can change these at any point in time to invalidate all existing cookies. This will force all users to have to log in again.
 *
 * @since 2.6.0
 */
define('AUTH_KEY', 'EsFfpZbWtQfPR3tBxKiUK9TM6uN8VhYxnMzHVfStqTktbGEwngtjaSXWaDmx676a');
define('SECURE_AUTH_KEY', 'fdcBGUFA35wfec95FYY5L6Srtj2F9zK9Dq3YyZvXb8Az7Fz4nge37jg9QjWQvraB');
define('LOGGED_IN_KEY', 'P79tQrTPrUrFsHauHxq4xmBTc82i92qPUzKu7KiWU2GyXRamgpN3KtS9h7rtYMK2');
define('NONCE_KEY', 'zPPvVM6wtBLc2bszZb3HCA84XVUpW3gmarMitHifHySR4sYhUU4mWsBUHB6FEBGM');
define('AUTH_SALT', 'kS8iQ5uqDBtx4mwcyxY2hBxNF4DZ2MxVfDgVjhhdSdvvUiShdEzVUvhN65sDh7mP');
define('SECURE_AUTH_SALT', 'eUiZv5QXvbmhKdDA2uLcVmxpYXxS6ikN3JS63ebFLUrrhZytBrKSj6GcruhrPp6f');
define('LOGGED_IN_SALT', 'wc66xwks5DeG454GQH7baaac7t3zaHHiVJuAWSvd3cziCgFeVMs5Vj3ceParWZYB');
define('NONCE_SALT', 'HwZw5zLkHFUqJ8zNHF9dRjEB25HUTL22Fa9AHP8AZKQyxtT2p5CUjytJfx6Q5qR9');

/**#@-*/

/**
 * WordPress Database Table prefix.
 *
 * You can have multiple installations in one database if you give each a unique
 * prefix. Only numbers, letters, and underscores please!
 */
$table_prefix  = 'wp_';

/**
 * WordPress Localized Language, defaults to English.
 *
 * Change this to localize WordPress. A corresponding MO file for the chosen
 * language must be installed to wp-content/languages. For example, install
 * de_DE.mo to wp-content/languages and set WPLANG to 'de_DE' to enable German
 * language support.
 */
define('WPLANG', '');

/**
 * For developers: WordPress debugging mode.
 *
 * Change this to true to enable the display of notices during development.
 * It is strongly recommended that plugin and theme developers use WP_DEBUG
 * in their development environments.
 */
define('WP_DEBUG', false);

/* That's all, stop editing! Happy blogging. */

/** Absolute path to the WordPress directory. */
if ( !defined('ABSPATH') )
	define('ABSPATH', dirname(__FILE__) . '/');

/** enable wp-super-cache */
define('WP_CACHE', true);

/** enable direct filesystem access for theme/plugin installs */
define('FS_METHOD', 'direct');

define('WPCACHEHOME',"/usr/home/sync23/public_html/synchronicityy.stream/wp-content/plugins/wp-super-cache/");
/** Sets up WordPress vars and included files. */
require_once(ABSPATH . 'wp-settings.php');
