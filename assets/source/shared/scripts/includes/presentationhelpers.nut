function getButtonTags()
{
	local tags = [];
	tags.push(::getPlatformString());
	tags.push(::getControllerTypeString());
	tags.push("controls_" + ::getGamepadControlSchemeString());
	return tags;
}

function addButtonTags(p_presentation)
{
	::addTagsToPresentation(p_presentation, ::getButtonTags());
}

function addTagsToPresentation(p_presentation, p_tags)
{
	if (p_presentation != null && p_tags != null)
	{
		foreach(tag in p_tags)
		{
			p_presentation.addTag(tag);
		}
	}
}

function presentationFileExists(p_file)
{
	if (p_file == null || typeof(p_file) != "string") return false;
	
	// strip the presentaiton folder if the filename is prefixed with it
	if (p_file.slice(0, 13) == "presentation/") p_file = p_file.slice(13);
	
	return getPresentationNames().find(p_file) != null;
}
