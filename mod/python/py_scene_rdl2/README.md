Python Bindings for scene_rdl2
==============================

py_scene_rdl2 delivers Python bindings for scene_rdl2 using boost::python.
These were originally delivered with the usd_rdl2 package but delivering them
with scene_rdl2 makes more sense, so they were moved here.

Usage
=====

Python bindings are delivered in the **scene_rdl2** python package.

API usage generally follows the scene_rdl2 C++ APIs.

Start with a rez2 environment that includes moonshine or moonbase_proxies:

```bash
rez2
rez-env scene_rdl2 moonshine
python
```

Then set up a SceneContext and get to work:

```python
import scene_rdl2

context = scene_rdl2.SceneContext()
context.setProxyModeEnabled(True)  # set this to use proxy mode
context.loadAllSceneClasses()

camera = context.createSceneObject("PerspectiveCamera", "shot_cam")
camera.set("focal", 24.98)
camera.set("mb_shutter_open", -0.6)
camera.set("mb_shutter_close", 0)

scenevars = context.getSceneVariables()
scenevars.set("camera", camera)

scene_rdl2.writeSceneToFile(context, "example.rdla")
```

This code creates and outputs example.rdla:

```lua
SceneVariables {
    ["camera"] = PerspectiveCamera("shot_cam"),
}

PerspectiveCamera("shot_cam") {
    ["mb_shutter_open"] = -0.600000024,
    ["mb_shutter_close"] = 0,
    ["focal"] = blur(24.9799995, 24.9799995),
}
```
