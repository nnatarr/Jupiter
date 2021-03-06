
entity main_entity is
    port (
        x0, x1, x2, x3: in BIT;
        y: out BIT;
    );
end asd;

architecture main_entity_arch of main_entity is
    component 2band2_or2 port(tx1, tx2, tx3, tx4: in BIT; txout: out BIT);
    end component;

    signal net_1, eubs0, eubs1, eubs3, net_0: BIT;
begin
    UBS0: 2band2_or2 port map(0, net_0, 0, net_1, y);
    UBS1: 2band2_or2 port map(x2, x3, 1, 1, net_1);
    EUBS0: 2band2_or2 port map(1, 0, x0, x1, eubs0);
    EUBS1: 2band2_or2 port map(1, 1, 1, 0, eubs1);
    EUBS3: 2band2_or2 port map(eubs0, eubs1, 1, 0, eubs3);
    EUBS2: 2band2_or2 port map(eubs3, 0, 0, 1, net_0);
end architecture main_entity_arch;