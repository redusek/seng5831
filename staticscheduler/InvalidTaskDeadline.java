package staticscheduler;


@SuppressWarnings("serial")
public class InvalidTaskDeadline extends TaskException
{
	public InvalidTaskDeadline()
	{
		super("Task deadline must be greater than the sum of WCET and offset.");
	}
}