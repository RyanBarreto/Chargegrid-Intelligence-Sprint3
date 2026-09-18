-- Garante que o aplicativo web consiga atualizar o status do pagamento
CREATE POLICY "Permitir update do App Web" 
ON public.Estacoes 
FOR UPDATE 
USING (true)
WITH CHECK (true);
