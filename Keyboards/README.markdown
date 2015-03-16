Keyboard Compiler Scripts
=========================

Scripts for major keyboards designed using the Kiibohd firmware.
Please refer to `<script> --help` for specific details.

Refer to the [wiki](https://github.com/kiibohd/controller/wiki) on setting up your system for compiling.


Build Steps
-----------

* Try to build once to make sure your system is setup correctly
* Add any .kll files in the build directory you want
* Edit `<script>` to include the new .kll files
* Rebuild


Example
-------

```bash
./infinity.bash
```


Projects
--------

* infinity.bash (Infinity Keyboard 2014/10/15)
* template.bash (Example template for new keyboards)


**Extra files**

* cmake.bash (Used by the compilation script, does nothing on it's own)

