Description:
===========

Paranoia module is for all the sysadmins out there who think that
allowing random CMS admins to execute PHP of their choice is not
a safe idea.

What it does:
-------------

- Save the permissions form once to remove all previous grants.
  (An error appears in the site status report if a role still has this
  permission.)
- Disable granting to Anonymous or Authenticated any permission that is
  marked "restrict access" in a module's hook_permission.
- Disable granting several permissions from popular contributed modules
  that are not marked as "restrict access" but are still important.
- Remove the paranoia module from the module administration page.
- Provides a hook to let you remove other modules from the module
  administration page.

NOTE on disabling:
------------------

The only way to disable paranoia module is by changing its status in the
database system table.  By design it does not show up in the module
administration page after it is enabled.

`UPDATE system SET status = 0 WHERE name = 'paranoia';`

Current Maintainer
------------------

- David Norman (https://github.com/deekayen)

Maintainers
-----------

- Originally written for Drupal by Gerhard Killesreiter and
  Greg Knaddison @greggles
- Ported to Backdrop by David Norman (https://github.com/deekayen)

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.