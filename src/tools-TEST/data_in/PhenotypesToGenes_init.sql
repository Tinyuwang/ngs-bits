
INSERT INTO `gene`(`id`, `hgnc_id`, `symbol`, `name`, `type`) VALUES 
('407590','25662','AAGAB','alpha and gamma adaptin binding protein','protein-coding gene'),
('407865','145','ACTG2','actin gamma 2, smooth muscle','protein-coding gene'),
('410012','1100','BRCA1','BRCA1 DNA repair associated','protein-coding gene'),
('410154','1148','BUB1','BUB1 mitotic checkpoint serine/threonine kinase','protein-coding gene');

INSERT INTO `hpo_term` (`id`, `hpo_id`, `name`, `definition`, `synonyms`) VALUES
(11495, 'HP:0000003', 'Multicystic kidney dysplasia', '\"Multicystic dysplasia of the kidney is characterized by multiple cysts of varying size in the kidney and the absence of a normal pelvocaliceal system. The condition is associated with ureteral or ureteropelvic atresia, and the affected kidney is nonfunctional.', ''),
(64350, 'HP:0002862', 'Bladder carcinoma', '\"The presence of a carcinoma of the urinary bladder.', '');

INSERT INTO `hpo_genes` (`hpo_term_id`, `gene`) VALUES
(11495, 'BUB1'),
(11495, 'ACTG2'),
(64350, 'BUB1'),
(64350, 'AAGAB');
