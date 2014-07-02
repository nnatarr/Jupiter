
entity main_entity is
    port (
        x0, x1, x2, x3: in BIT;
        y: out BIT;
    );
end entity;

architecture main_entity_arch of main_entity is
    component 2band2_or2 port(tx1, tx2, tx3, tx4: in BIT; txout: out BIT);
    end component;

    signal net_1, eubs0, eubs1, eubs2, net_0: BIT;
begin
    2band2_or2 port map(0, net_0, 0, net_1, y);
    2band2_or2 port map(x2, x3, 1, 1, net_1);
    2band2_or2 port map(x0, x1, 1, 0, eubs0);
    2band2_or2 port map(1, 1, 1, 0, eubs1);
    2band2_or2 port map(1, 0, eubs0, eubs1, eubs2);
    2band2_or2 port map(1, 1, 0, eubs2, net_0);
end architecture main_entity_arch;