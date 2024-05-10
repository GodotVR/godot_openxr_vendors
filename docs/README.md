Documentation
=============

How to build the documentation
------------------------------

### One-time setup ###

Install all the necessary Python dependencies for Sphinx:

```bash
python -m venv python.env
. python.env/bin/activate
pip install -r requirements.txt
```

Next time, before you run any of the commands below, make sure to run this in your terminal first:

```bash
. python.env/bin/activate
```

### Updating the XML of the API docs ###

It's possible to use Godot's `--doctool` to automatically update the XML for any new classes
or members, by running:

```bash
../scripts/update_doc_classes_xml.sh
```

If your Godot isn't called `godot` or isn't on your `PATH`, then set the `GODOT` variable to
point to your Godot 4 executable, for example:

```bash
export GODOT=/path/to/godot
../scripts/update_doc_classes_xml.sh
```

Then you can edit the XML documentation in `../doc_classes`!

### Generating the RST from the XML ###

To convert the API docs from XML into RST for Sphinx, run:

```bash
make api
```

### Generating the HTML documentation ###

To use Sphinx to generate the HTML documentation, run:

```bash
make html
```

Then you can open `_build/html/index.html` a web browser!

Sphinx isn't that great at detecting changes, so if it doesn't seem to be picking up your edits,
try running:

```bash
make clean html
```
