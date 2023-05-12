#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_USER 30
#define KERNEL_PORT 0
#define PAYLOAD_SIZE 100

struct sock *nl_sk = NULL;
void nl_msg_callback(struct sk_buff*);

/* Initialize the netlink config with callback */
struct netlink_kernel_cfg nl_cfg = {
    .input = nl_msg_callback,
};

void nl_msg_callback(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    char msg_buf[PAYLOAD_SIZE];
    int res;

    pr_info("Callback %s invoked\n", __FUNCTION__);
    memset(&msg_buf, 0, PAYLOAD_SIZE);

    /* Extract the data starting with the netlink header. skb->head and skb->tail should also yield the same result */
    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received payload: %s\n", (char *)nlmsg_data(nlh));
    /* Get the sending process PID from the netlink header attached with the recieved payload */
    pid = nlh->nlmsg_pid;

    sprintf(msg_buf, "I got your message PID %d. Hello from kernel", pid);

    /* Allocate a new socket buffer and memory for output data. Link the allocated data address to the socket buffer */
    skb_out = nlmsg_new(PAYLOAD_SIZE, GFP_KERNEL);
    if (!skb_out){
        pr_err("Failed to allocate new socket buffer\n");
        return;
    }

    /* Add the netlink message (header + payload space without data) to the tail of the output socket buffer */
    nlh = nlmsg_put(skb_out, KERNEL_PORT, 0, NLMSG_DONE, PAYLOAD_SIZE, 0);
    /* Set the dst_group member of the netlink socket buffer params struct to 0, indicating it as unicast */
    NETLINK_CB(skb_out).dst_group = 0;
    /* Copy the message data to the socket buffer netlink payload space */
    strncpy(nlmsg_data(nlh), msg_buf, PAYLOAD_SIZE);

    /* Send the unicast message to the user process */
    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        pr_err("Error while sending back to user\n");
}

static int __init netlink_mod_init(void)
{

    pr_info("Initializing netlink kernel module\n");

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &nl_cfg);
    if (!nl_sk){
        pr_err("Error creating kernel netlink socket\n");
        return -10;
    }

    return 0;
}

static void __exit netlink_mod_exit(void)
{
    pr_info("Exiting netlink module\n");
    netlink_kernel_release(nl_sk);
}

module_init(netlink_mod_init);
module_exit(netlink_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amol Dhamale");
MODULE_DESCRIPTION("An example netlink socket kernel module");