DELETE FROM command WHERE name = 'azth maxskill';
INSERT INTO command VALUES('azth maxskill', 1002, 'Max all skill of targeted player');
DELETE FROM trinity_string WHERE entry = 90001;
INSERT INTO trinity_string(entry, content_default) VALUES(90001, 'The target level must be level %u');