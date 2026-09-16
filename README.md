# ChargeGrid Intelligence – Gestão de Recarga EV (Sprint 3)

INTEGRANTES DO GRUPO:
- Fernando Henrique Lembo - RM: 570228
- Guilherme Lopes Muniz - RM: 569521
- Gustavo Russo Balizardo - RM: 569283
- Ryan Barreto Carlos Dias - RM: 574126


## Objetivos

- Demonstrar a integração prática dos pilares de Tarifação, Pagamento e Interoperabilidade do ChargeGrid Intelligence mapeados nas Sprints 1 e 2.
- Implementar um fluxo End-to-End: confirmação de transação no aplicativo móvel, atualização do estado no banco de dados Cloud (Supabase), com resposta automática e liberação no hardware embarcado (ESP32).
- Prover interface visual de feedback ao usuário final através de um Display de mensagens ("Pagamento Aprovado / Recarga Liberada").

## Arquitetura do Sistema e Esquema de Integração

### Diagrama de Fluxo dos Componentes

[ App Movel ] -> [ Supabase Cloud ] -> [ ESP32 ] -> [ Display LCD ] -> [ Módulo Relé ]

### Descrição dos Módulos

1. Aplicativo Móvel: Interface onde o usuário realiza o pagamento e solicita a recarga.
2. Supabase Cloud: Banco de dados em nuvem que recebe a confirmação do pagamento e envia o evento em tempo real.
3. ESP32: Microcontrolador conectado ao Wi-Fi que escuta as atualizações do Supabase.
4. Display LCD: Exibe o status da transação para o usuário ("Pagamento Aprovado").
5. Módulo Relé: Atuador elétrico que liga a energia da estação de recarga após a aprovação.

### Justificativa Técnica das Escolhas

* ESP32: Escolhido por possuir Wi-Fi nativo, alto desempenho de processamento e baixo custo para escutar eventos em nuvem.
* Supabase Cloud: Utilizado como banco de dados em nuvem pela facilidade de envio de eventos em tempo real para o hardware assim que a transação ocorre.
* Display LCD (I2C): Utilizado por permitir comunicação com poucos fios (SDA/SCL) e fornecer feedback claro e imediato para o usuário na estação.
* Atuador / Botão de Liberação (Simulação do Relé): Componente responsável por simular o acionamento e a liberação física da corrente na estação de recarga após a validação do pagamento.
