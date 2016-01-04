DELETE FROM rbac_permissions WHERE id IN (1001, 1002);
INSERT INTO rbac_permissions VALUES(1001, 'Command azth'), (1002, 'Command azth custom maxskill');
DELETE FROM rbac_linked_permissions WHERE linkedId IN (1001, 1002);
INSERT INTO rbac_linked_permissions VALUES(192, 1001), (192, 1002);