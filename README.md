Clark's Palace
==============

This role deploys required applications for Clark's Palace to a remote system.

Requirements
------------

One requirement is to have a system with a configured instance of Ansible. This means that you have an inventory and access to the remote system via SSH with keys.

1. Add the remote system to the *hosts* file in the group [palace]
2. For every system you want to manage, you need to have the master's SSH key in the *authorized_keys* file of the managed/remote system.

From the management system for the user **root**:

```bash
$ sudo ssh-copy-id -i /home/[your local user]/.ssh/id_rsa.pub pi@[IP address of remote system]
```

While checking if you are able to login the remote system without password, you should check if Python and the DNF Python binding are available. If not, install them to save some time later.

```bash
$ sudo dnf -y install python python-dnf
```


3. Perform a first check if Ansible works. You need to use the user `root`:

```bash
$ ansible -i hosts palace -m ping -u pi -b
192.168.1.117 | success >> {
    "changed": false,
    "ping": "pong"
}
```

For further details about the setup of Ansible, check their [documentation](http://docs.ansible.com/ansible/).

Role Variables
--------------

- **ha_venv**: The location of the virtual environment. Defaults to `/opt/home-assistant`
- **palace_user**: The user for Clark's Palace. Defaults to `palace`.
- **ha_port**: The default port which is 8123. If you change that, you will need to modify the `configuration.yml` file of Home Assistant.
- **pkgs**: List of packages to install on the remote system.


Dependencies
------------

There are no dependencies for this role.

Example Playbook
----------------

Including an example of how to use your role (for instance, with variables passed in as parameters) is always nice for users too:

```bash
- hosts: palace
  roles:
     - home-assistant.home-assistant-ansible
```

Warning
-------

**Think first** before you implement stuff from this repository. Consider the playbooks in this repository as non-generic. They were made especially for the deployment of Home Assistant and Fedora at the moment.

Author Information
------------------

Jonathan Clark.

