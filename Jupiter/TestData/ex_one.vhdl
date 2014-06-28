entity or2 is
    port (
        ab : in std_ulogic_vector(1 downto 0);
        z : out BIT;
    );
end entity;

architecture or2_arch of or2 is
begin
    process (ab) is
    begin
        case ab of
            when "00" => z <= "0";
            when "01" => z <= "1";
            when "10" => z <= "1";
            when "11" => z <= "1";
        end case;
    end process;
end architecture or2_arch; 

entity not_and2 is
    port (
        ab : in std_ulogic_vector(1 downto 0);
        z : out BIT;
    );
end entity;

architecture not_and2_arch of not_and2 is
begin
    process (ab) is
    begin
        case ab of
            when "00" => z <= "1";
            when "01" => z <= "0";
            when "10" => z <= "0";
            when "11" => z <= "0";
        end case;
    end process;
end architecture not_and2_arch; 

entity result_entity is
    port (
        a,b,c,d : in BIT;
        z : out BIT;
    );
end entity;

architecture result_entity_arch of result_entity is
    component or2 port(X1, X2: in BIT; Z: out BIT);
    end component;

    component not_and2 port(X1, X2: in BIT; Z: out BIT);
    end component;

    signal s_one, s_two: BIT;
begin
    not_and2 port map (a, b, s_one);
    not_and2 port map (c, d, s_two);
    or2 port map (s_one, s_two, z);
end architecture result_entity_arch; 