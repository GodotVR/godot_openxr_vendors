extends PanelContainer

const CHOICE = preload("res://menu_3d/2D/choice.tscn")

var _current_choice_idx: int = -1

@onready var _choices = $Labels/Choices


func on_pinch_tapped(hand: OpenXRHand.Hands):
	if hand == OpenXRHand.Hands.HAND_LEFT:
		_on_user_highlight_next_choice()
	else:
		_on_user_select_choice()


## Set the title text for the list of choices
func set_title_text(text: String) -> void:
	$Labels/Title.text = text


## Add a choice with the given text.[br]
## [param callable] will be called when this choice is chosen.
func add_choice(text: String, callable: Callable) -> Label:
	if !callable.is_valid():
		printerr("add_choice expects a valid callable")
		return

	var new_choice: Label = CHOICE.instantiate()
	new_choice.set_meta("callable", callable)
	_choices.add_child(new_choice)
	new_choice.text = str(_choices.get_children().size()) + ": " + text

	if _current_choice_idx == -1:
		_on_user_highlight_next_choice()

	return new_choice


## Clear all choices; call [method add_choice] to re-add choices.
func clear_choices() -> void:
	for child in _choices.get_children():
		child.get_parent().remove_child(child)
		child.queue_free()

	_current_choice_idx = -1


## Retrieve the current choice (i.e the currently-highlighted choice).[br]
## Returns [code]-1[/code] if there are no choices
func get_current_choice_idx() -> int:
	return _current_choice_idx


## Set the current choice (i.e the currently-highlighted choice).[br]
## The choice will be clamped if it is out-of-bounds.
func set_current_choice_idx(new_current_choice_idx: int) -> void:
	if _choices.get_child_count() <= 0:
		return

	# remove the highlight from the current choice
	_highlight_choice(false)

	_current_choice_idx = clamp(new_current_choice_idx, 0, _choices.get_child_count() - 1)

	# then highlight the new choice (it might be the same choice that was just unhighlighted, which is
	# fine)
	_highlight_choice(true)


func _on_user_select_choice():
	if _current_choice_idx < 0 || _choices.get_child_count() <= _current_choice_idx:
		printerr("unable to select choice because there are no choices")
		return

	# call the callable; it should be provided but check that it's valid anyway
	var choice: Label = _choices.get_children()[_current_choice_idx]
	var choice_callable: Callable = choice.get_meta("callable", Callable())
	if choice_callable.is_valid():
		choice_callable.call()


func _on_user_highlight_next_choice():
	if _choices.get_child_count() <= 0:
		printerr("unable to highlight next choice because there are no choices")
		return

	# remove highlight from the current choice
	_highlight_choice(false)

	# bump to next choice
	_current_choice_idx += 1

	# wrap around if at the end
	if _choices.get_child_count() <= _current_choice_idx:
		_current_choice_idx = 0

	# highlight the new choice
	_highlight_choice(true)


func _highlight_choice(highlight: bool):
	if 0 <= _current_choice_idx && _current_choice_idx < _choices.get_child_count():
		_choices.get_children()[_current_choice_idx].get_theme_stylebox("normal").draw_center = highlight
