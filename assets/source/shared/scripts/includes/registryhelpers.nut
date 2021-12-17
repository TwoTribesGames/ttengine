tt_include("includes/logs");

function logRegistry()
{
	log("Registry contents (" + getRegistry().size() + "):");
	for (local i = 0; i < getRegistry().size(); i++)
	{
		local key = getRegistry().getKeyAt(i);
		::echo(getRegistry().get(key), key);
	}
	log("");
}

function logPersistentRegistry()
{
	log("Persistent Registry contents (" + getRegistry().size() + "):");
	for (local i = 0; i < getRegistry().sizePersistent(); i++)
	{
		local key = getRegistry().getKeyPersistentAt(i);
		::echo(getRegistry().getPersistent(key), key);
	}
	log("");
}

function logRegistries()
{
	logRegistry();
	logPersistentRegistry();
}