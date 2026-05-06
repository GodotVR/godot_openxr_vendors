extends PanelContainer
class_name Menu2D

@onready var _choices = $Labels/Choices


## Set the title text for the list of choices
func set_title_text(text: String) -> void:
	$Labels/Title.text = text


## Add a choice with the given text.[br]
## [param callable] will be called when this choice is chosen.
func add_choice(text: String, callable: Callable):
	if !callable.is_valid():
		printerr("add_choice expects a valid callable")
		return

	var new_choice: Button = Button.new()
	new_choice.alignment = HORIZONTAL_ALIGNMENT_LEFT
	new_choice.pressed.connect(callable)
	_choices.add_child(new_choice)
	new_choice.text = str(_choices.get_children().size()) + ": " + text


## Clear all choices; call [method add_choice] to re-add choices.
func clear_choices() -> void:
	for child in _choices.get_children():
		child.get_parent().remove_child(child)
		child.queue_free()
