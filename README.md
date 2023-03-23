Paranoia
========

Impose additional restrictions on the granting of elevated privileges to
anonymous and authenticated user roles.

What it does:
-------------

- Save the permissions form once to remove all previous grants.
- Disable granting to Anonymous or Authenticated any permission that is
  marked "restrict access" in a module's hook_permission.
- Disable granting several permissions from popular contributed modules
  that are not marked as "restrict access" but are still important.
- Remove the paranoia module from the module administration page.
- Provides a hook to let you remove other modules from the module
  administration page.
- When changing your password, it destroys all sessions other than the
  session used to change the password.

NOTE on disabling:
------------------

The Paranoia module cannot be disabled within the user interface; by design it
does not show up in the module administration page after it is enabled. The
module can be disabled in one of two ways:
1. Changing the module status in the database `system` table:
```sql
    UPDATE system SET status = 0 WHERE name = 'paranoia';
```
2. Use [Bee](https://github.com/backdrop-contrib/bee), the Backdrop command line tool, to disable the module:
```
    bee disable paranoia
```

Current Maintainer
------------------

- [Laryn Kragt Bakker](https://github.com/laryn), [CEDC.org](https://CEDC.org)
- Collaboration and co-maintainers welcome!

Credits
-------

- Originally written for Drupal by Gerhard Killesreiter and
  [Greg Knaddison](https://github.com/greggles)
- Ported to Backdrop by [David Norman](https://github.com/deekayen)

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
